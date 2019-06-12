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
		ImGui::Text("changelog release v2.1 WIP b1");
		ImGui::Text("- 	removed mouse movements");
		ImGui::Text("- 	removed player list");
		ImGui::Text("- 	added rounded corners and fixed other gui issues");
		ImGui::Text("- 	fixed the menu issues.");
		ImGui::Text("- 	relocated fakelag to the hvh tab");
		ImGui::Text("- 	relocated fakelag slider ammount to the hvh tab");
		ImGui::Text("- 	fixed name changer");
		ImGui::Text("- 	fixed auto shoot for some weapons");
		ImGui::Text("- 	added chams types for weapons and fixed chams alpha on localplayer");
		ImGui::Text("- 	fixed Arms color doesn't change on wireframe mode");
		ImGui::Text("- 	added meme french bomb so now its le bom");
		ImGui::Text("----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");
		ImGui::Text("SPECIAL THANKS!");
		ImGui::Text("Koutsie, Koutsie#5777 -Staying up till 5am helping me find a nice menu & color scheme.");
		ImGui::Text("Localplayer, Localplayer#0218 -The OG who actually made spacehook and the guy im pasting into his src, sorry xo");	
		ImGui::End();
	}
}
