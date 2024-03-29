#include "bank.hxx"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <sys/shm.h>


static constexpr int FTOK_KEY{9};

static const std::string FREEZE{"f"};
static const std::string UNFREEZE{"u"};
static const std::string SEND_MONEY{"s"};
static const std::string TRANSACTION_TO_EVERY_VALID_CUSTOMER{"t"};
static const std::string SET_MINIMUM_ALLOWED{"smin"};
static const std::string SET_MAXIMUM_ALLOWED{"smax"};

void print_command_info() {
    std::cout << "commands: \n"
              << "\tfreeze:\n\t\t" << FREEZE << " `index`\n"
              << "\tunfreeze:\n\t\t" << UNFREEZE << " `index`\n"
              << "\tsend money:\n\t\t" << SEND_MONEY
              << " `index from` `index to` `transaction value`\n"
              << "\ttransaction to every valid customer:\n\t\t"
              << TRANSACTION_TO_EVERY_VALID_CUSTOMER << " `transaction value`\n"
              << "\tset minimum allowed:\n\t\t" << SET_MINIMUM_ALLOWED
              << " `index` `minimum balance`\n"
              << "\tset maximum allowed:\n\t\t" << SET_MAXIMUM_ALLOWED
              << " `index` `maximum balance`\n";
}

int client(char** argv) {
    auto token = ftok(argv[1], FTOK_KEY);
    Mutex mutex(argv[1]);
    int shmid = shmget(token, 0, 0600);
    if (shmid == -1) {
        throw std::logic_error{"shmget"};
    }
    auto bank_storage = reinterpret_cast<BankStorage*>(
                            shmat(shmid, nullptr, 0)
                        );
    if (reinterpret_cast<void*>(bank_storage) == reinterpret_cast<void*>(-1)) {
        std::logic_error{"shmat error"};
    }

    Bank bank{bank_storage, mutex};

    std::string log{};
    while (std::cin) {
        std::system("clear");
        bank.print();
        print_command_info();
        std::cout << log;
        std::cout << ">" << std::flush;
        log += '>';
        std::string command;
        std::cin >> command;
        log += command;
        try {
            if (command == FREEZE) {
                std::size_t index;
                std::cin >> index;
                log += ' ';
                log += std::to_string(index);
                bank.freeze(index);
            } else if (command == UNFREEZE) {
                std::size_t index;
                std::cin >> index;
                log += ' ';
                log += std::to_string(index);
                bank.unfreeze(index);
            } else if (command == SEND_MONEY) {
                std::size_t from, to;
                Customer::balance_t value;
                std::cin >> from >> to >> value;
                log += ' ';
                log += std::to_string(from);
                log += ' ';
                log += std::to_string(to);
                log += ' ';
                log += std::to_string(value);
                if (value < 0) {
                    std::cerr << "no negative money\n";
                    continue;
                }
                bank.send_money(from, to, value);
            } else if (command == TRANSACTION_TO_EVERY_VALID_CUSTOMER) {
                Customer::balance_t value;
                std::cin >> value;
                log += ' ';
                log += std::to_string(value);
                if (value < 0) {
                    std::cerr << "no negative money\n";
                    continue;
                }
                bank.transaction_to_every_valid_customer(value);
            } else if (command == SET_MINIMUM_ALLOWED) {
                std::size_t index;
                Customer::balance_t minimum_allowed;
                std::cin >> index >> minimum_allowed;
                log += ' ';
                log += std::to_string(index);
                log += ' ';
                log += std::to_string(minimum_allowed);
                bank.set_minimum_allowed(minimum_allowed, index);
            } else if (command == SET_MAXIMUM_ALLOWED) {
                std::size_t index;
                Customer::balance_t maximum_allowed;
                std::cin >> index >> maximum_allowed;
                log += ' ';
                log += std::to_string(index);
                log += ' ';
                log += std::to_string(maximum_allowed);
                bank.set_maximum_allowed(maximum_allowed, index);
            } else {
                std::logic_error{"not command `" + command + "` was found"};
            }
        } catch (const std::exception& e) {
            mutex.unlock();
            log += "\n\t";
            log += e.what();
        }
        log += '\n';
    }

    return 0;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        throw std::invalid_argument{"client must have 2 arguments"};
    }
    return client(argv);
}
