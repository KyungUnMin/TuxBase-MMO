#include <filesystem>
#include <iostream>

int main()
{
  std::cout << "Hello TuxBase-MMO GameServer" << std::endl;
  std::cout << "Current Directory: " << std::filesystem::current_path() << std::endl;
  std::cout << "Hello Anti"<< std::endl;

  return 0;
}
