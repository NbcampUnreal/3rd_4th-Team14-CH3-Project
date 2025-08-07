#include "GtDamageReceiverComponent.h"

UGtDamageReceiverComponent::UGtDamageReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

// 인터페이스 함수의 실제 구현부
bool UGtDamageReceiverComponent::ApplyDamage_Implementation(const FGtDamageInfo& DamageInfo, FGtDamageResult& OutDamageResult)
{
	UE_LOG(LogTemp, Log, TEXT("[DamageReceiverComponent] Received %.2f base damage."), DamageInfo.BaseDamage);

	// 1. 최종 데미지 계산 (지금은 단순하게 처리)
	// TODO: 여기에 방어력, 저항, 버프/디버프 등을 고려한 복잡한 데미지 계산 로직 구현
	OutDamageResult.FinalDamage = DamageInfo.BaseDamage;
	UE_LOG(LogTemp, Log, TEXT("[DamageReceiverComponent] Calculated %.2f final damage."), OutDamageResult.FinalDamage);

	// 2. 데미지 처리 완료 신호를 외부(캐릭터 등)에 알림 (Broadcast)
	OnDamageProcessed.Broadcast(OutDamageResult);
    
	return true;
}
