#pragma once

#include "hook_util.h"

namespace UPC
{

enum class ProductType
{
	App = 1,
	DLC = 2,
	Item = 4
};

struct Product
{
	Product(uint32_t appid, ProductType type)
	{
		this->appid = appid;
		this->type = type;
		this->mystery1 = type == ProductType::Item ? 4 : 1;
		this->mystery2 = type == ProductType::Item ? 1 : 3;
	}

	unsigned int appid;
	ProductType type;
	unsigned int mystery1;
	unsigned int always_3 = 3;
	unsigned int always_0 = 0;
	unsigned int mystery2;
};

struct ProductList
{
	unsigned int length = 0;
	unsigned int padding = 0; // What is this? offset?
	Product** data = NULL; // Array of pointers
};

typedef void (*UplayCallback)(unsigned long, void*);


struct CallbackContainer
{
	void* context = NULL;
	UplayCallback originalCallback = NULL;
	void* callbackData = NULL;
	ProductList* legitProductList = NULL;
};

}

extern vector<unsigned int> dlcs;
extern vector<unsigned int> items;

int UPC_Init(unsigned int version, unsigned int appid);
int UPC_ProductListFree(void* context, UPC::ProductList* inProductList);
int UPC_ProductListGet(
	void* context,
	char* inOptUserIdUtf8,
	unsigned int filter,
	UPC::ProductList** outProductList,
	UPC::UplayCallback callback,
	void* callbackData
);
