#include "ProjectGASEnemyCharacter.h"
#include "../AbilitySystem/Attributes/ProjectGASAttributeSet_Enemy.h"

AProjectGASEnemyCharacter::AProjectGASEnemyCharacter()
{
	AttributeSet = CreateDefaultSubobject<UProjectGASAttributeSet_Enemy>(TEXT("AttributeSet"));
	// ASC는 베이스 클래스 생성자에서 이미 생성됨.
	// 메시/AnimBP/콜리전은 이 클래스를 부모로 하는 블루프린트에서 지정.
}