// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "HttpPrivatePCH.h"
#include "HttpManager.h"

// FHttpManager

FCriticalSection FHttpManager::RequestLock;

FHttpManager::FHttpManager()
	:	FTickerObjectBase(0.0f)
	,	DeferredDestroyDelay(10.0f)
{
	
}

FHttpManager::~FHttpManager()
{
	
}

bool FHttpManager::Tick(float DeltaSeconds)
{
	FScopeLock ScopeLock(&RequestLock);

	// Tick each active request
	for (TArray<TSharedRef<IHttpRequest>>::TIterator It(Requests); It; ++It)
	{
		TSharedRef<IHttpRequest> Request = *It;
		Request->Tick(DeltaSeconds);
	}
	// Tick any pending destroy objects
	for (int Idx=0; Idx < PendingDestroyRequests.Num(); Idx++)
	{
		FRequestPendingDestroy& Request = PendingDestroyRequests[Idx];
		Request.TimeLeft -= DeltaSeconds;
		if (Request.TimeLeft <= 0)
		{	
			PendingDestroyRequests.RemoveAt(Idx--);
		}		
	}
	// keep ticking
	return true;
}

void FHttpManager::AddRequest(const TSharedRef<IHttpRequest>& Request)
{
	FScopeLock ScopeLock(&RequestLock);

	Requests.Add(Request);
}

void FHttpManager::RemoveRequest(const TSharedRef<IHttpRequest>& Request)
{
	FScopeLock ScopeLock(&RequestLock);

	// Keep track of requests that have been removed to be destroyed later
	PendingDestroyRequests.AddUnique(FRequestPendingDestroy(DeferredDestroyDelay,Request));

	Requests.Remove(Request);
}

bool FHttpManager::IsValidRequest(const IHttpRequest* RequestPtr) const
{
	FScopeLock ScopeLock(&RequestLock);

	bool bResult = false;
	for (auto& Request : Requests)
	{
		if (&Request.Get() == RequestPtr)
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

void FHttpManager::DumpRequests(FOutputDevice& Ar) const
{
	FScopeLock ScopeLock(&RequestLock);
	
	Ar.Logf(TEXT("------- (%d) Http Requests"), Requests.Num());
	for (auto& Request : Requests)
	{
		Ar.Logf(TEXT("	verb=[%s] url=[%s] status=%s"),
			*Request->GetVerb(), *Request->GetURL(), EHttpRequestStatus::ToString(Request->GetStatus()));
	}
}

static FString GetUrlDomain(const FString& Url)
{
	FString Protocol;
	FString Domain = Url;
	// split the http protocol portion from domain
	Url.Split(TEXT("://"), &Protocol, &Domain);
	// strip off everything but the domain portion
	int32 Idx = Domain.Find(TEXT("/"));
	int32 IdxOpt = Domain.Find(TEXT("?"));
	Idx = IdxOpt >= 0 && IdxOpt < Idx ? IdxOpt : Idx;
	if (Idx > 0)
	{
		Domain = Domain.Left(Idx);
	}
	return Domain;
}
