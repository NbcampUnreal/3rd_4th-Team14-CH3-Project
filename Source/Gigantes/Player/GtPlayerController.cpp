// Fill out your copyright notice in the Description page of Project Settings.


#include "GtPlayerController.h"

#include "GtPlayerCameraManager.h"

AGtPlayerController::AGtPlayerController()
{
	PlayerCameraManagerClass = AGtPlayerCameraManager::StaticClass();
}
