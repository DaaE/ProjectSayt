#pragma once

// 매크로 설명:
// GAS의 Attribute는 보일러플레이트 코드가 엄청 많아요.
// 이 매크로가 Getter/Setter/Initter 함수들을 자동 생성해줍니다.
// Unity에서 { get; set; } 한 줄로 끝났던 걸 GAS에서는
// 네트워크 동기화 + 변경 감지를 위해 더 많은 코드가 필요해서 매크로로 압축한 거예요.
// 여러 AttributeSet 클래스가 공통으로 쓰는 접근자 매크로.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

