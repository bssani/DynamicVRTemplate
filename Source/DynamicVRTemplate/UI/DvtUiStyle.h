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
}
