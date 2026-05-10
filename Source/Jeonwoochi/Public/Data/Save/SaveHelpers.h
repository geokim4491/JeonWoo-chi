

#pragma once
#include "CoreMinimal.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// UObject → 바이트 (SaveGame 플래그 있는 필드만)
inline void JW_SerializeToBytes(UObject* Obj, TArray<uint8>& Out)
{
    Out.Reset();
    FMemoryWriter W(Out, true);
    FObjectAndNameAsStringProxyArchive Ar(W, true);
    Ar.ArIsSaveGame = true;       // ★ SaveGame 플래그 필터
    Obj->Serialize(Ar);
}

// 바이트 → UObject (SaveGame 플래그 있는 필드만)
inline void JW_DeserializeFromBytes(UObject* Obj, const TArray<uint8>& In)
{
    FMemoryReader R(In, true);
    FObjectAndNameAsStringProxyArchive Ar(R, true);
    Ar.ArIsSaveGame = true;
    Obj->Serialize(Ar);
}

// 해당 클래스에 SaveGame 필드가 하나라도 있는지
inline bool JW_HasSaveGameFields(const UStruct* S)
{
    for (TFieldIterator<FProperty> It(S, EFieldIteratorFlags::IncludeSuper); It; ++It)
    {
        if (It->HasAnyPropertyFlags(CPF_SaveGame)) return true;
    }
    return false;
}