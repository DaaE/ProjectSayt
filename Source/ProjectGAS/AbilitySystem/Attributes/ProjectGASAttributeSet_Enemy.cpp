#include "ProjectGASAttributeSet_Enemy.h"
#include "Net/UnrealNetwork.h"

UProjectGASAttributeSet_Enemy::UProjectGASAttributeSet_Enemy()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitDefense(0.f);
}

void UProjectGASAttributeSet_Enemy::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UProjectGASAttributeSet_Enemy, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UProjectGASAttributeSet_Enemy, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UProjectGASAttributeSet_Enemy, Defense, COND_None, REPNOTIFY_Always);
}

void UProjectGASAttributeSet_Enemy::OnRep_Health(
	const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(
		UProjectGASAttributeSet_Enemy, Health, OldHealth);
}

void UProjectGASAttributeSet_Enemy::OnRep_MaxHealth(
	const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(
		UProjectGASAttributeSet_Enemy, MaxHealth, OldMaxHealth);
}

void UProjectGASAttributeSet_Enemy::OnRep_Defense(
	const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(
		UProjectGASAttributeSet_Enemy, Defense, OldDefense);
}

void UProjectGASAttributeSet_Enemy::PreAttributeChange(
	const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}