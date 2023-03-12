#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "Player/Player.hpp"

//#define SEE_CALLSTACK_ON_EXCEPTION
//#define JUST_PRINT_SHIP_DATA

#ifdef JUST_PRINT_SHIP_DATA
#include "GameState/StatePrinting.hpp" 
#endif

int main()
{
	using namespace std::chrono_literals;
	constexpr auto PollRate = 32ms;
	constexpr auto InuputRate = 8ms;
	constexpr char PointerMapFile[] = "pointermap.cfg";

	std::chrono::high_resolution_clock clock;
	auto prevTime = clock.now();

	try
	{
		std::ifstream map(PointerMapFile);
		if (!map)
		{
			std::cout <<
				"Couldn't open " << PointerMapFile << ".\n"
				"Make sure it's present and make sure the application has appropriate permissions.\n";

			std::system("pause");
			return 1;
		}

		AddressData addresses;
		addresses.parse(map);

		StateReader reader(addresses);
		if (!reader)
		{
			std::cout << "Couldn't hook into the FTL process. Make sure exactly 1 instance of it is running.\n";

			std::system("pause");
			return 1;
		}

#ifdef JUST_PRINT_SHIP_DATA
		if (!reader.poll(true))
		{
			std::cout << "Failed to read memory!\n";
		}
		else
		{
			constexpr char ShipDataFile[] = "shipdata.txt";
			std::ofstream out(ShipDataFile);

			if (!out)
			{
				std::cout <<
					"Couldn't open " << ShipDataFile << " for writing.\n"
					"Make sure the application has appropriate permissions.\n";

				return 1;
			}

			auto& state = reader.getState();

			if (!state.inGame)
			{
				std::cout << "You need to be in game first.\n";
				return 1;
			}

			struct RoomPrettyID
			{
				SystemID sys = SystemID::Invalid;
				int diff = -1;
			};

			std::unordered_map<int, RoomPrettyID> roomMap;
			auto& rooms = state.player.rooms;
			auto& doors = state.player.doors;
			auto& ship = state.shipPosition;
			int curHall = 1, curArtillery = 1;

			for (auto& [id, room] : rooms)
			{
				if (room.system != SystemID::Invalid)
				{
					RoomPrettyID prettyId{ .sys = room.system };

					if (room.system == SystemID::Artillery)
					{
						if(curArtillery > 1)
							prettyId.diff = curArtillery;

						++curArtillery;
					}

					roomMap.emplace(id, prettyId);
				}
				else
				{
					roomMap.emplace(id, RoomPrettyID{ .diff = curHall });
					++curHall;
				}
			}

			auto printPrettyId = [&](auto& id) {
				switch (id.sys)
				{
				case SystemID::Invalid: out << "hall"; break;
				case SystemID::Artillery: out << id.sys; break;
				default: out << id.sys; break;
				}

				if (id.diff > 0) out << id.diff;

				out << ' ';
			};

			out << "ROOMS\n";

			for (auto& [id, prettyId] : roomMap)
			{
				auto& room = rooms.at(id);

				printPrettyId(prettyId);

				out << (room.position.x + ship.x) << ' '
					<< (room.position.y + ship.y) << ' '
					<< (room.size.x * RoomTileLength) << ' '
					<< (room.size.y * RoomTileLength) << '\n';
			}

			out << "\nDOORS\n";

			for (auto& [id, door] : doors)
			{
				if (door.roomA < 0) out << "outside ";
				else printPrettyId(roomMap.at(door.roomA));

				if (door.roomB < 0) out << "outside ";
				else printPrettyId(roomMap.at(door.roomB));

				out << (door.position.x + ship.x) << ' '
					<< (door.position.y + ship.y) << '\n';
			}

			out << '\n';

			return 0;
		}
#else
		Player player(reader);

		while (reader)
		{
			if (!reader.poll())
			{
				std::cout << "Failed to read memory!\n";
				break;
			}

			player.onLoop(int(PollRate.count()));

			std::this_thread::sleep_for(PollRate);
		}
#endif
	}

#ifdef SEE_CALLSTACK_ON_EXCEPTION
	catch (bool) {} // hacky, I know
#else
	catch (const std::exception& e)
	{
		std::cout << "An error occurred: " << e.what() << '\n';
		std::system("pause");
		return 1;
	}
#endif

#ifndef JUST_PRINT_SHIP_DATA
	std::cout << "FTL process closed!\n";
#endif

	std::system("pause");
}