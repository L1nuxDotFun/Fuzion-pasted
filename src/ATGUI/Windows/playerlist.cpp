#include "playerlist.h"

bool PlayerList::showWindow = false;

void PlayerList::RenderWindow()
{
	if( Settings::UI::Windows::Playerlist::reload )
	{
		ImGui::SetNextWindowPos(ImVec2(Settings::UI::Windows::Playerlist::posX, Settings::UI::Windows::Playerlist::posY), ImGuiSetCond_Always);
		ImGui::SetNextWindowSize(ImVec2(Settings::UI::Windows::Playerlist::sizeX, Settings::UI::Windows::Playerlist::sizeY), ImGuiSetCond_Always);
		Settings::UI::Windows::Playerlist::reload = false;
		PlayerList::showWindow = Settings::UI::Windows::Playerlist::open;
	}
	else
	{
		ImGui::SetNextWindowPos(ImVec2(Settings::UI::Windows::Playerlist::posX, Settings::UI::Windows::Playerlist::posY), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(Settings::UI::Windows::Playerlist::sizeX, Settings::UI::Windows::Playerlist::sizeY), ImGuiSetCond_FirstUseEver);
	}
	if (!PlayerList::showWindow)
	{
		Settings::UI::Windows::Playerlist::open = false;
		return;
	}

	if (ImGui::Begin(XORSTR("About"), &PlayerList::showWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
	{
		ImGui::Text("changelog release v2.0 WIP b7");
		ImGui::Text("- removed mouse movements");
		ImGui::Text("- removed player list");
		ImGui::Text("- added rounded corners and fixed other gui issues");
		ImGui::End();
	}
}
