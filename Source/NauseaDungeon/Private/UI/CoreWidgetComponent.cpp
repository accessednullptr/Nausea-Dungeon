// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CoreWidgetComponent.h"

UCoreWidgetComponent::UCoreWidgetComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UCoreWidgetComponent::InitWidget()
{
	Super::InitWidget();

	if (UCoreUserWidget* CoreUserWidget = Cast<UCoreUserWidget>(GetWidget()))
	{
		CoreUserWidget->InitializeWidgetComponent(this);
	}
}