#pragma once

#include "CoreMinimal.h"

namespace DvtUiStyle
{
    inline FLinearColor Color(const TCHAR* Hex) { return FLinearColor::FromSRGBColor(FColor::FromHex(Hex)); }
    inline const FLinearColor Background = Color(TEXT("131a26"));
    inline const FLinearColor Text = Color(TEXT("eef2f7"));
    inline const FLinearColor SecondaryText = Color(TEXT("9aa6b8"));
    inline const FLinearColor Line = Color(TEXT("2e3a4e"));
    inline const FLinearColor Accent = Color(TEXT("5b8cff"));
    inline const FLinearColor Window = Color(TEXT("0b0f17"));
    inline const FLinearColor Selected = Color(TEXT("1c2534"));
    inline const FLinearColor ButtonOutline = Color(TEXT("66748a"));
    inline const FLinearColor Header = Color(TEXT("0d2a66"));
    inline const FLinearColor Applied = Color(TEXT("3cc285"));
    inline const FLinearColor NeedsCheck = Color(TEXT("f2b544"));
}
