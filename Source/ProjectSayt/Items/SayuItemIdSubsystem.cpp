// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuItemIdSubsystem.h"

int32 USayuItemIdSubsystem::IssueNextItemInstanceId()
{
	return NextItemInstanceId++;
}
