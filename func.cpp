#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <cmath>
#include <iomanip>
#include "header.h"


// Объявление глобальной переменной баланса
int current_balance = 1000;

// Константа для имени файла сохранения
const std::string save_file_name = "save.txt";

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool loadGameState() {
    std::ifstream file(save_file_name); // Открываем только для чтения
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << save_file_name << std::endl;
        // Если файла нет, сбрасываем игру в начальное состояние
        return resetGameState();
    }

    std::string first_line;
    if (std::getline(file, first_line)) {
        try {
            current_balance = std::stoi(first_line);
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error: The first line of the file is not an integer. Resetting to default." << std::endl;
            file.close();
            return resetGameState();
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Error: The number in the first line is too large. Resetting to default." << std::endl;
            file.close();
            return resetGameState();
        }
    }
    else {
        std::cout << "Save file is empty. Resetting to default." << std::endl;
        file.close();
        return resetGameState();
    }

    file.close();
    std::cout << "Game state loaded from file." << std::endl;
    return true;
}

// Функция для сброса состояния игры в начальное состояние
bool resetGameState() {
    std::ofstream file(save_file_name, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << save_file_name << std::endl;
        return false;
    }

    file << 1000 << std::endl;
    current_balance = 1000;
    file.close();
    clearScreen();
    std::cout << "Game state reset to default." << std::endl;
    return true;
}

// Функция для обновления файла сохранения. Принимает на вход ИЗМНЕНИЕ баланса + записывает последнюю строчку игры в историю
bool updateSaveFile(int balance_change) {
    std::fstream file(save_file_name, std::ios::in | std::ios::out);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading/writing: " << save_file_name << std::endl;
        return false;
    }

    current_balance += balance_change;

    file.seekp(0, std::ios::beg);
    file << current_balance << std::endl;
    file.flush();

    // Перемещаемся в конец файла для добавления записи об изменении баланса
    file.seekp(0, std::ios::end);

    if (balance_change > 0) {
        file << "+" << balance_change << std::endl;
    }
    else if (balance_change < 0) {
        file << balance_change << std::endl;
    }
    else {
        file << "+0" << std::endl;
    }

    file.flush();
    file.close();
    return true;
}



// ==================== СЛОТ-МАШИНА ====================
void findCluster(const std::vector<std::vector<char>>& reels, int row, int col, char symbol, std::vector<std::pair<int, int>>& cluster, std::vector<std::vector<bool>>& visited, const std::vector<int>& megawaysCols) {
    int ROWS = static_cast<int>(reels.size());
    int COLS = static_cast<int>(reels[0].size());

    // Проверка, находится ли ячейка в допустимых пределах высоты столбца
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS || row >= megawaysCols[col] || visited[row][col] || reels[row][col] != symbol) {
        return;
    }

    visited[row][col] = true;
    cluster.push_back({ row, col });

    // Рекурсивный поиск в соседних ячейках (вверх, вниз, влево, вправо)
    findCluster(reels, row + 1, col, symbol, cluster, visited, megawaysCols); // Вниз
    findCluster(reels, row - 1, col, symbol, cluster, visited, megawaysCols); // Вверх
    findCluster(reels, row, col + 1, symbol, cluster, visited, megawaysCols); // Вправо
    findCluster(reels, row, col - 1, symbol, cluster, visited, megawaysCols); // Влево
}

bool isCluster(const std::vector<std::vector<char>>& reels, int row, int col, char symbol, std::vector<std::pair<int, int>>& cluster, std::vector<std::vector<bool>>& visited, const std::vector<int>& megawaysCols) {
    cluster.clear();
    findCluster(reels, row, col, symbol, cluster, visited, megawaysCols);
    return !cluster.empty();
}

int playSlots() {
    srand(time(0));

    const int ROWS = 5;
    const int COLS = 6;
    const std::string SYMBOLS = "ABCDEF";
    const int MIN_CLUSTER_SIZE = 3;

    double scatterProb = 0.05;

    const double LINE_PAYOUT_MULTIPLIERS[] = { 0, 0.2, 0.5, 1, 2, 5 };
    const double CLUSTER_PAYOUT_MULTIPLIERS[] = { 0.5, 0.9, 1.5, 2, 4, 8, 12, 16, 20, 25 };
    const double SCATTER_PAYOUT_MULTIPLIER = 0.8;

    // Проверка баланса
    if (current_balance < 10) {
        std::cout << "Seems like somebody is out of carrots(" << std::endl;
        std::cout << "Minimum bet: 10" << std::endl;
        std::cout << "Wanna reset game stats? (y/n): ";
        char confirm;
        std::cin >> confirm;
        if (confirm == 'y') {
            if (!resetGameState()) {
                std::cerr << "Unable to reset game stats!" << std::endl;
            }
            else {
                std::cout << "Game progress reset completed" << std::endl;
            }
        }
        else {
            std::cout << "Reset cancelled" << std::endl;
        }
        return 0; // Выходим из функции, если баланс = 0
    }

    int bet;
    std::cout << "Enter your bet(10-" << current_balance << "): ";
    std::cin >> bet;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Your bet is " << bet << " carrots" << std::endl;
    // Запрос ставки
    while (bet <= 10 || bet > current_balance) {
        if (std::cin.fail()) {
            clearScreen();
            std::cout << "Invalid input" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
           
        }
        else {
            std::cout << "Your bet must be between 10 and " << current_balance << " carrots or enter 0 to exit" << std::endl;
            std::cin >> bet;
            if (bet == 0) {
                clearScreen();
                return 0;
            }
        }
    }
    updateSaveFile(-bet);

    // Пошло-поехало
    std::vector<int> megawaysCols(COLS);
    int totalWays = 1;
    for (int i = 0; i < COLS; ++i) {
        megawaysCols[i] = (rand() % (ROWS - 1)) + 2; // Min 2, Max ROWS
        totalWays *= megawaysCols[i];
    }

    // Генерация символов
    std::vector<std::vector<char>> reels(ROWS, std::vector<char>(COLS, ' '));
    std::vector<std::vector<char>> megawaysReels(ROWS, std::vector<char>(COLS, ' '));
    for (int col = 0; col < COLS; ++col) {
        for (int row = 0; row < megawaysCols[col]; ++row) {
            double randomValue = (double)rand() / RAND_MAX;
            char symbol;
            if (randomValue < scatterProb) {
                symbol = 'S';
            }
            else {
                int symbolIndex = rand() % SYMBOLS.length();
                symbol = SYMBOLS[symbolIndex];
            }
            megawaysReels[row][col] = symbol;
        }
    }

    // Вывод
    clearScreen();
    std::cout << "Your bet is " << bet << " carrots" << std::endl << std::endl;
    std::cout << "--------------------MEGAWAYS-------------------" << std::endl;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << megawaysReels[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------------------------------------" << std::endl;

    // Рассчет выигрыша
    int winnings = 0;
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));

    // Скаттер
    int scatterWinnings = 0;
    int scatterCount = 0;
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (megawaysReels[row][col] == 'S') {
                scatterCount++;
                visited[row][col] = true; //Помечаем скаттеры как посещенные, чтоб не мешали поиску кластеров и линий
            }
        }
    }
    if (scatterCount >= 3) {
        scatterWinnings = bet * (scatterCount * SCATTER_PAYOUT_MULTIPLIER);
        std::cout << "Scatter winning! (" << scatterCount << " symbols): " << scatterWinnings << std::endl;
        winnings += scatterWinnings;
    }

    // Кластер
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (!visited[row][col]) {
                std::vector<std::pair<int, int>> cluster;
                char symbol = megawaysReels[row][col];
                int clusterWinnings = 0;

                if (row < megawaysCols[col] && isCluster(megawaysReels, row, col, symbol, cluster, visited, megawaysCols) && cluster.size() >= MIN_CLUSTER_SIZE) {
                    int clusterSize = cluster.size() - MIN_CLUSTER_SIZE;
                    clusterWinnings = bet * CLUSTER_PAYOUT_MULTIPLIERS[clusterSize];
                    std::cout << "Cluster - winning! (Size: " << cluster.size() << " of symbol: " << symbol << "): " << clusterWinnings << std::endl;
                    winnings += clusterWinnings;

                    for (const auto& cell : cluster) {
                        visited[cell.first][cell.second] = true;
                    }
                }
            }
        }
    }
    std::cout << std::endl;
    // Линейный выигрыш
    for (int row = 0; row < ROWS; ++row) {
        if (megawaysCols[0] > row && !visited[row][0]) { // Проверяем, что строка не часть кластера
            char firstSymbol = megawaysReels[row][0];
            int winningSymbols = 1;
            bool isLine = true;
            int lineWinnings = 0;
            for (int col = 1; col < COLS; ++col) {
                if (megawaysCols[col] <= row || megawaysReels[row][col] != firstSymbol || visited[row][col]) { //проверяем посещение
                    isLine = false;
                    break;
                }
                winningSymbols++;
            }

            if (winningSymbols >= 3 && isLine) { // Проверка линейного выигрыша и что это не часть кластера
                lineWinnings = bet * LINE_PAYOUT_MULTIPLIERS[winningSymbols - 3];
                std::cout << "Winning line in " << row + 1 << " ( " << winningSymbols << "of symbol): " << lineWinnings << std::endl;
                winnings += lineWinnings;
            }
        }
    }
    if (winnings == 0) {
        std::cout << "Total winnings: " << winnings << " carrots. Unlucky for you" << std::endl;
    }
    else if (winnings <= bet) {
        std::cout << "Total winnings: " << winnings << " carrots. Better that nothing!" << std::endl;
    }
    else if (winnings > bet * 2) {
        std::cout << "BIG WIN OF: " << winnings << " CARROTS!!! Your luck is incredible!" << std::endl;
    }
    else if (winnings > bet) {
        std::cout << "Total winnings: " << winnings << " carrots! It is your day!" << std::endl;
    }
    std::cout << std::endl;
    updateSaveFile(winnings);


    return winnings;
}

void displayBetHistory() {
    std::ifstream saveFile(save_file_name);
    if (!saveFile.is_open()) {
        std::cerr << "Error: Could not open save file '" << save_file_name << "'." << std::endl;
        return;
    }

    std::string line;
    // Пропускаем первую строку (баланс)
    std::getline(saveFile, line);


    const int maxLinesToCheck = 20;
    bool isEmpty = true;
    int initialPos = saveFile.tellg(); 

    for (int i = 0; i < maxLinesToCheck; ++i) {
        if (!std::getline(saveFile, line)) {
            break; 
        }

        std::stringstream ss(line);
        std::string trimmedLine;
        ss >> trimmedLine;

        if (!trimmedLine.empty()) {
            isEmpty = false;
            break;
        }
    }

    if (isEmpty) {
        std::cout << "Game history is empty. Play some games so see some!" << std::endl;
        saveFile.close();
        return;
    }

    saveFile.clear(); 
    saveFile.seekg(initialPos, std::ios::beg);


    int stake;
    int winnings;
    bool stakeFound = false;
    bool errorFound = false;
    std::vector<int> betVec;
    std::vector<int> winVec;

    while (std::getline(saveFile, line)) {
        if (line.empty()) {
            continue;
        }
        try {
            int value = std::stoi(line);

            if (value < 0) {
                if (stakeFound) {
                    std::cerr << "Error: Multiple stakes found consecutively in save file(Stake: " << stake << ", next: " << value << "). Save file might be corrupted. Try resetting game data" << std::endl;
                    errorFound = true;
                    break;
                }
                stake = value;
                betVec.push_back(stake);
                stakeFound = true;
            }
            else if (value >= 0) {
                if (!stakeFound) {
                    if (!winVec.empty() && winVec.back() > 0) {
                        std::cerr << "Error: Multiple winnings found consecutively in save file. Save file might be corrupted. Try resetting game data" << std::endl;
                        errorFound = true;
                        break;
                    }
                    std::cerr << "Error: Winnings found before stake in save file. Save file might be corrupted. Try resetting game data" << std::endl;
                    errorFound = true;
                    break;
                }
                winnings = value;
                winVec.push_back(winnings);
                stakeFound = false;
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid data in save file. Save file might be corrupted. Try resetting game data" << std::endl;
            errorFound = true;
            break;
        }
    }
    if (!errorFound) {
        if (stakeFound) {
            std::cerr << "Error: Stake found without winnings at the end of the file. Save file might be corrupted." << std::endl;
        }
        for (size_t i = 0; i < betVec.size(); ++i) {
            std::cout << std::left << "Bet: " << std::setw(20) << abs(betVec[i]) << "Winnings: " << std::setw(20) << winVec[i] << std::endl;
        }
    }

    saveFile.close();

}


int startGame() {
    if (!loadGameState()) {
        std::cerr << "Failed to initialize game state. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Welcome to the Slot Simulator!" << std::endl;
    std::cout << "Here you can win a lot of fresh yummy carrots" << std::endl;
    int choice;

    do {
        std::cout << "Current balance: " << current_balance << " carrots" << std::endl;
        std::cout << "1. Play Slots" << std::endl;
        std::cout << "2. Display bet history" << std::endl;
        std::cout << "3. Save and Exit" << std::endl;
        std::cout << "4. Reset Game (and clear save)" << std::endl;
        std::cout << "Enter your choice (1 - 4): ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (std::cin.fail()) {
            clearScreen();
            std::cin.clear();
            std::cout << "Invalid input" << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
            choice = 0; 
            continue; 
        }
        switch (choice) {
        case 1:
            clearScreen();
            playSlots();
            break;
        case 2:
            clearScreen();
            displayBetHistory();
            std::cout << std::endl;
            break;
        case 3:
            clearScreen();
            std::cout << "Saving game and exiting..." << std::endl;
            break;
        case 4:
            std::cout << "Are you sure you want to reset the game? (y/n): ";
            char confirm;
            std::cin >> confirm;
            if (confirm == 'y') {
                if (!resetGameState()) {
                    std::cerr << "Failed to reset game state!" << std::endl;
                }
            }
            else {
                clearScreen();
                std::cout << "Reset cancelled." << std::endl;
            }
            break;
        default:
            clearScreen();
            std::cout << "Invalid input. Please enter a number from 1 to 4: " << std::endl;
            
        }
    } while (choice != 3);

    return 0;
}
