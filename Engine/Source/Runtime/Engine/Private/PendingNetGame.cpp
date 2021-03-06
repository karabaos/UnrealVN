// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
  PendingNetGame.cpp: Unreal pending net game class.
=============================================================================*/

#include "EnginePrivate.h"
#include "Net/UnrealNetwork.h"
#include "Net/NetworkProfiler.h"
#include "Net/DataChannel.h"
#include "GeneralProjectSettings.h"

void UPendingNetGame::Initialize(const FURL& InURL)
{
	NetDriver = NULL;
	URL = InURL;
	bSuccessfullyConnected = false;
	bSentJoinRequest = false;
}

UPendingNetGame::UPendingNetGame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPendingNetGame::InitNetDriver()
{
	if (!GDisallowNetworkTravel)
	{
		NETWORK_PROFILER(GNetworkProfiler.TrackSessionChange(true, URL));

		// Try to create network driver.
		if (GEngine->CreateNamedNetDriver(this, NAME_PendingNetDriver, NAME_GameNetDriver))
		{
			NetDriver = GEngine->FindNamedNetDriver(this, NAME_PendingNetDriver);
		}
		check(NetDriver);

		if( NetDriver->InitConnect( this, URL, ConnectionError ) )
		{
			// Send initial message.
			uint8 IsLittleEndian = uint8(PLATFORM_LITTLE_ENDIAN);
			check(IsLittleEndian == !!IsLittleEndian); // should only be one or zero
			
			uint32 LocalNetworkVersion = FNetworkVersion::GetLocalNetworkVersion();

			FNetControlMessage<NMT_Hello>::Send( NetDriver->ServerConnection, IsLittleEndian, LocalNetworkVersion );

			NetDriver->ServerConnection->FlushNet();
		}
		else
		{
			// error initializing the network stack...
			UE_LOG(LogNet, Warning, TEXT("error initializing the network stack"));
			GEngine->DestroyNamedNetDriver(this, NetDriver->NetDriverName);
			NetDriver = NULL;

			// ConnectionError should be set by calling InitConnect...however, if we set NetDriver to NULL without setting a
			// value for ConnectionError, we'll trigger the assertion at the top of UPendingNetGame::Tick() so make sure it's set
			if ( ConnectionError.Len() == 0 )
			{
				ConnectionError = NSLOCTEXT("Engine", "NetworkInit", "Error initializing network layer.").ToString();
			}
		}
	}
	else
	{
		ConnectionError = NSLOCTEXT("Engine", "UsedCheatCommands", "Console commands were used which are disallowed in netplay.  You must restart the game to create a match.").ToString();
	}
}

void UPendingNetGame::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << NetDriver;
	}
}

void UPendingNetGame::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{	
	UPendingNetGame* This = CastChecked<UPendingNetGame>(InThis);
#if WITH_EDITOR
	if( GIsEditor )
	{
		// Required by the unified GC when running in the editor
		Collector.AddReferencedObject( This->NetDriver, This );
	}
#endif
	Super::AddReferencedObjects( This, Collector );
}

EAcceptConnection::Type UPendingNetGame::NotifyAcceptingConnection()
{
	return EAcceptConnection::Reject; 
}
void UPendingNetGame::NotifyAcceptedConnection( class UNetConnection* Connection )
{
}
bool UPendingNetGame::NotifyAcceptingChannel( class UChannel* Channel )
{
	return 0;
}

void UPendingNetGame::NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, class FInBunch& Bunch)
{
	check(Connection==NetDriver->ServerConnection);

#if !UE_BUILD_SHIPPING
	UE_LOG(LogNet, Verbose, TEXT("PendingLevel received: %s"), FNetControlMessageInfo::GetName(MessageType));
#endif

	// This client got a response from the server.
	switch (MessageType)
	{
		case NMT_Upgrade:
			// Report mismatch.
			uint32 RemoteNetworkVersion;
			FNetControlMessage<NMT_Upgrade>::Receive(Bunch, RemoteNetworkVersion);
			// Upgrade
			ConnectionError = NSLOCTEXT("Engine", "ClientOutdated", "The match you are trying to join is running an incompatible version of the game.  Please try upgrading your game version.").ToString();
			GEngine->BroadcastNetworkFailure(NULL, NetDriver, ENetworkFailure::OutdatedClient, ConnectionError);
			break;

		case NMT_Failure:
		{
			// our connection attempt failed for some reason, for example a synchronization mismatch (bad GUID, etc) or because the server rejected our join attempt (too many players, etc)
			// here we can further parse the string to determine the reason that the server closed our connection and present it to the user

			FString ErrorMsg;
			FNetControlMessage<NMT_Failure>::Receive(Bunch, ErrorMsg);
			if (ErrorMsg.IsEmpty())
			{
				ErrorMsg = NSLOCTEXT("NetworkErrors", "GenericPendingConnectionFailed", "Pending Connection Failed.").ToString();
			}
			
			// This error will be resolved in TickWorldTravel()
			ConnectionError = ErrorMsg;

			// Force close the session
			UE_LOG(LogNet, Log, TEXT("NetConnection::Close() [%s] [%s] [%s] from NMT_Failure %s"), 
				Connection->Driver ? *Connection->Driver->NetDriverName.ToString() : TEXT("NULL"),
				Connection->PlayerController ? *Connection->PlayerController->GetName() : TEXT("NoPC"),
				Connection->OwningActor ? *Connection->OwningActor->GetName() : TEXT("No Owner"),
				*ConnectionError);

			Connection->Close();
			break;
		}
		case NMT_Challenge:
		{
			// Challenged by server.
			FNetControlMessage<NMT_Challenge>::Receive(Bunch, Connection->Challenge);

			FURL PartialURL(URL);
			PartialURL.Host = TEXT("");
			PartialURL.Port = PartialURL.UrlConfig.DefaultPort; // HACK: Need to fix URL parsing 
			for (int32 i = URL.Op.Num() - 1; i >= 0; i--)
			{
				if (URL.Op[i].Left(5) == TEXT("game="))
				{
					URL.Op.RemoveAt(i);
				}
			}

			FUniqueNetIdRepl UniqueIdRepl;

			ULocalPlayer* LocalPlayer = GEngine->GetFirstGamePlayer(this);
			if (LocalPlayer)
			{
				// Send the player nickname if available
				FString OverrideName = LocalPlayer->GetNickname();
				if (OverrideName.Len() > 0)
				{
					PartialURL.AddOption(*FString::Printf(TEXT("Name=%s"), *OverrideName));
				}

				// Send any game-specific url options for this player
				FString GameUrlOptions = LocalPlayer->GetGameLoginOptions();
				if (GameUrlOptions.Len() > 0)
				{
					PartialURL.AddOption(*FString::Printf(TEXT("%s"), *GameUrlOptions));
				}

				// Send the player unique Id at login
				UniqueIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			
			Connection->ClientResponse = TEXT("0");
			FString URLString(PartialURL.ToString());
			FNetControlMessage<NMT_Login>::Send(Connection, Connection->ClientResponse, URLString, UniqueIdRepl);
			NetDriver->ServerConnection->FlushNet();
			break;
		}
		case NMT_Welcome:
		{
			// Server accepted connection.
			FString GameName;
			FString RedirectURL;

			FNetControlMessage<NMT_Welcome>::Receive(Bunch, URL.Map, GameName, RedirectURL);

			//GEngine->NetworkRemapPath(this, URL.Map);

			UE_LOG(LogNet, Log, TEXT("Welcomed by server (Level: %s, Game: %s)"), *URL.Map, *GameName);

			// extract map name and options
			{
				FURL DefaultURL;
				FURL TempURL( &DefaultURL, *URL.Map, TRAVEL_Partial );
				URL.Map = TempURL.Map;
				URL.RedirectURL = RedirectURL;
				URL.Op.Append(TempURL.Op);
			}

			if (GameName.Len() > 0)
			{
				URL.AddOption(*FString::Printf(TEXT("game=%s"), *GameName));
			}

			// Send out netspeed now that we're connected
			FNetControlMessage<NMT_Netspeed>::Send(Connection, Connection->CurrentNetSpeed);

			// We have successfully connected.
			bSuccessfullyConnected = true;
			break;
		}
		case NMT_NetGUIDAssign:
		{
			FNetworkGUID NetGUID;
			FString Path;
			FNetControlMessage<NMT_NetGUIDAssign>::Receive(Bunch, NetGUID, Path);
			NetDriver->ServerConnection->PackageMap->ResolvePathAndAssignNetGUID(NetGUID, Path);
			break;
		}
		default:
			UE_LOG(LogNet, Log, TEXT(" --- Unknown/unexpected message for pending level"));
			break;
	}
}

void UPendingNetGame::Tick( float DeltaTime )
{
	check(NetDriver);
	check(NetDriver->ServerConnection);

	// Handle timed out or failed connection.
	if( NetDriver->ServerConnection->State==USOCK_Closed && ConnectionError==TEXT("") )
	{
		ConnectionError = NSLOCTEXT("Engine", "ConnectionFailed", "Your connection to the host has been lost.").ToString();
		return;
	}

	// Update network driver (may NULL itself via CancelPending if a disconnect/error occurs)
	NetDriver->TickDispatch(DeltaTime);
	if (NetDriver)
	{
		NetDriver->TickFlush(DeltaTime);
		if (NetDriver)
		{
			NetDriver->PostTickFlush();
		}
	}
}

void UPendingNetGame::SendJoin()
{
	bSentJoinRequest = true;

	FNetControlMessage<NMT_Join>::Send(NetDriver->ServerConnection);
	NetDriver->ServerConnection->FlushNet(true);
}
