#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include "header.h"

using namespace std;

// Объявление глобальной переменной баланса
int current_balance = 1000;

// Константа для имени файла сохранения
const std::string save_file_name = "save.txt";

// Функция для инициализации баланса из файла или сброса к заводским настройкам
/*bool initializeGameState(bool resetToDefault = false) {
    std::fstream file(save_file_name, std::ios::in | std::ios::out | std::ios::trunc); 
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << save_file_name << std::endl;
        return false;
    }

    if (resetToDefault) {
        file << 1000 << std::endl;
        current_balance = 1000;
        std::cout << "Game state reset to default." << std::endl;
    }
    else {
        std::string first_line;
        if (std::getline(file, first_line)) {
            try {
                current_balance = std::stoi(first_line);
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "Error: The first line of the file is not an integer. Resetting to default." << std::endl;
                file.seekp(0, std::ios::beg);
                file << 1000 << std::endl;
                file.close();
                return initializeGameState(true);


            }
            catch (const std::out_of_range& e) {
                std::cerr << "Error: The number in the first line is too large. Resetting to default." << std::endl;
                file.seekp(0, std::ios::beg);
                file << 1000 << std::endl;
                file.close();
                return initializeGameState(true);
            }
        }
        else {
            std::cout << "Save file is empty. Resetting to default." << std::endl;
            file.seekp(0, std::ios::beg);
            file << 1000 << std::endl;

            file.close();
            return initializeGameState(true);

        }
    }

    file.close();
    return true;
}
*/
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
            file.close(); // Закрываем файл перед сбросом
            return resetGameState();
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Error: The number in the first line is too large. Resetting to default." << std::endl;
            file.close(); // Закрываем файл перед сбросом
            return resetGameState();
        }
    }
    else {
        std::cout << "Save file is empty. Resetting to default." << std::endl;
        file.close(); // Закрываем файл перед сбросом
        return resetGameState();
    }

    file.close();
    std::cout << "Game state loaded from file." << std::endl;
    return true;
}

// Функция для сброса состояния игры в начальное состояние
bool resetGameState() {
    std::ofstream file(save_file_name, std::ios::trunc); // Открываем с trunc для очистки
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << save_file_name << std::endl;
        return false;
    }

    file << 1000 << std::endl;
    current_balance = 1000;
    file.close();
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
    file.flush(); //Сброс буфера

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

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ==================== СЛОТ-МАШИНА ====================
void findCluster(const vector<vector<char>>& reels, int row, int col, char symbol, vector<pair<int, int>>& cluster, vector<vector<bool>>& visited, const vector<int>& megawaysCols) {
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

bool isCluster(const vector<vector<char>>& reels, int row, int col, char symbol, vector<pair<int, int>>& cluster, vector<vector<bool>>& visited, const vector<int>& megawaysCols) {
    cluster.clear();
    findCluster(reels, row, col, symbol, cluster, visited, megawaysCols);
    return !cluster.empty();
}

int playSlots() {
    srand(time(0));

    const int ROWS = 5;
    const int COLS = 6;
    const string SYMBOLS = "ABCDEF";
    const int MIN_CLUSTER_SIZE = 3;

    double scatterProb = 0.05;

    const double LINE_PAYOUT_MULTIPLIERS[] = { 0, 0.2, 0.5, 1, 2, 5 };   
    const double CLUSTER_PAYOUT_MULTIPLIERS[] = { 0.5, 0.9, 1.5, 2, 4, 8, 12, 16, 20, 25 };  
    const double SCATTER_PAYOUT_MULTIPLIER = 0.8;  

    // Проверка баланса
    if (current_balance < 10) {
        cout << "Not enough carrots! Minimum bet: 10\n";
        cout << "Wanna reset game stats? (y/n): ";
        char confirm;
        cin >> confirm;
        if (confirm == 'y') {
            if (!initializeGameState(true)) {
                cerr << "Unable to reset game stats!" << endl;
            }
            else {
                cout << "Game progress reset completed" << endl;
            }
        }
        else {
            cout << "Reset cancelled" << endl;
        }
        return 0; // Важно: выходим из функции, если баланс = 0
    }

    int bet;
    cout << "Enter your bet: ";
    cin >> bet;
    cout << "Your bet is " << bet << " carrots" << endl;

    // Запрос ставки
    while (bet <= 0 || bet > current_balance) {
        cout << "Not enough carrots!" << endl << "Your bet must be between 10 and " << current_balance << " carrots or enter 0 to exit" << endl;
        cin >> bet;
        if (bet == 0) {
            return 0;
        }
    }
    updateSaveFile(-bet);

    // Пошло-поехало
    vector<int> megawaysCols(COLS);
    int totalWays = 1;
    for (int i = 0; i < COLS; ++i) {
        megawaysCols[i] = (rand() % (ROWS - 1)) + 2; // Min 2, Max ROWS
        totalWays *= megawaysCols[i];
    }

    // Генерация символов
    vector<vector<char>> reels(ROWS, vector<char>(COLS, ' '));
    vector<vector<char>> megawaysReels(ROWS, vector<char>(COLS, ' '));
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
    cout << "--------------------MEGAWAYS-------------------" << endl;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            cout << megawaysReels[i][j] << " ";
        }
        cout << endl;
    }
    cout << "--------------------------------------------------" << endl;

    // Рассчет выигрыша
    int winnings = 0;
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));

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
        cout << "Scatter winning! (" << scatterCount << " symbols): " << scatterWinnings << endl;
        winnings += scatterWinnings;
    }

    // Кластер
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (!visited[row][col]) {
                vector<pair<int, int>> cluster;
                char symbol = megawaysReels[row][col];
                int clusterWinnings = 0;

                if (row < megawaysCols[col] && isCluster(megawaysReels, row, col, symbol, cluster, visited, megawaysCols) && cluster.size() >= MIN_CLUSTER_SIZE) {
                    int clusterSize = cluster.size() - MIN_CLUSTER_SIZE;
                    clusterWinnings = bet * CLUSTER_PAYOUT_MULTIPLIERS[clusterSize];
                    cout << "Cluster - winning! (Size: " << cluster.size() << " of symbol: " << symbol << "): " << clusterWinnings << endl;
                    winnings += clusterWinnings;

                    for (const auto& cell : cluster) {
                        visited[cell.first][cell.second] = true;
                    }
                }
            }
        }
    }

    // Линейный выигрыш
    for (int row = 0; row < ROWS; ++row) {
        if (megawaysCols[0] > row && !visited[row][0]) { // Проверяем, что строка не часть кластера
            char firstSymbol = megawaysReels[row][0];
            int winningSymbols = 1;
            bool isLine = true;  // Флаг
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
                cout << "Big line in " << row + 1 << " ( " << winningSymbols << "of symbol): " << lineWinnings << endl;
                winnings += lineWinnings;
            }
        }
    }
    cout << "Total winnings: " << winnings << " carrots" << endl;
    updateSaveFile(winnings);


    return winnings;
}


int startGame() {
    if (!loadGameState()) {
        std::cerr << "Failed to initialize game state. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Welcome to the Slot Simulator!" << std::endl;
    std::cout << "Here you can win a lot of fresh yummy carrots" << std::endl;
    char choice;

    do {
        std::cout << "Current balance: " << current_balance << " carrots" << std::endl;
        std::cout << "1. Play Slots" << std::endl;
        std::cout << "2. Save and Exit" << std::endl;
        std::cout << "3. Reset Game (and Clear Save)" << std::endl;
        std::cout << "Enter your choice (1, 2, or 3): ";
        std::cin >> choice;

        switch (choice) {
        case '1':
            playSlots();
            break;
        case '2':
            if (updateSaveFile(0)) { // save and exit to save the final balance
                std::cout << "Saving game and exiting..." << std::endl;
            }
            else {
                std::cerr << "Error: Could not save the game" << std::endl;
            }
            break;
        case '3':
            std::cout << "Are you sure you want to reset the game? (y/n): ";
            char confirm;
            std::cin >> confirm;
            if (confirm == 'y') {
                if (!resetGameState()) {
                    std::cerr << "Failed to reset game state!" << std::endl;
                }
            }
            else {
                std::cout << "Reset cancelled." << std::endl;
            }
            break;
        default:
            std::cout << "Invalid choice. Please enter 1, 2, or 3" << std::endl;

        }
    } while (choice != '2');

    return 0;
}
