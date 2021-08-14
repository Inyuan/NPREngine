#include "UpdateData.h"
UpdateData* UpdateData::UPDInstance = nullptr;

UpdateData* UpdateData::GetInstance()
{
	if (!UPDInstance)
	{
		UPDInstance = new UpdateData();
	}
	return UPDInstance;
}

UpdateData::~UpdateData()
{
	if (UPDInstance)
	{
		delete(UPDInstance);
		UPDInstance = nullptr;
	}
};
