// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuItemIdSubsystem.h"

int32 USayuItemIdSubsystem::IssueNextItemInstanceId()
{
	return NextItemInstanceId++;
}
