#pragma once

namespace IntegrationWizard
{

void install();
void remove();

}

enum class Action
{
	NO_ACTION = 1000,
	UNEXPECTED_ERROR = 1001,
	INSTALL_INTEGRATIONS = 1002,
	REMOVE_INTEGRATIONS = 1003,
};
