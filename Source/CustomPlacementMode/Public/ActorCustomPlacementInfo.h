// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class FActorCustomPlacementInfo
{
public:

	FActorCustomPlacementInfo(const FString& InObjectPath, const FString& InFactory)
		: ObjectPath(InObjectPath)
		, Factory(InFactory)
	{
	}

	FActorCustomPlacementInfo(const FString& String)
	{
		if (!String.Split(FString(TEXT(";")), &ObjectPath, &Factory))
		{
			ObjectPath = String;
		}
	}

	bool operator==(const FActorCustomPlacementInfo& Other) const
	{
		return ObjectPath == Other.ObjectPath;
	}

	bool operator!=(const FActorCustomPlacementInfo& Other) const
	{
		return ObjectPath != Other.ObjectPath;
	}

	FString ToString() const
	{
		return ObjectPath + TEXT(";") + Factory;
	}


public:

	FString ObjectPath;
	FString Factory;
};