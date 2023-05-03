#include "bank.hxx"
#include "bank_storage.hxx"
#include <iostream>
Bank::Bank(const BankStorage* storage, const Mutex& mutex) : _storage{storage}, _mutex(mutex) {}
Customer::BalanceInfo Bank::get_balance_info(size_t index) const {
   _mutex.lock();
   auto balance_info = _storage->at(index).get_balance_info();
   _mutex.unlock();
   return balance_info;
}
void Bank::freeze(size_t index) {
   _mutex.lock();
  _storage->at(index).freeze();
   _mutex.unlock();
}
void Bank::unfreeze(size_t index) {
   _mutex.lock();
  _storage->at(index).unfreeze();
   _mutex.unlock();
}
void Bank::send_money(size_t from, size_t to,
                              Customer::balance_t value) {
   _mutex.lock();
  if (value <= 0) {
    throw std::logic_error{"Sent value cannot be non positive"};
  }

  auto &sender = _storage->at(from);
  auto &receiver = _storage->at(to);
  sender.check_is_not_frozen();
  receiver.check_is_not_frozen();
  if (sender.is_valid_transaction(-value) and
      receiver.is_valid_transaction(value)) {
    sender.unsafe_transaction(-value);
    receiver.unsafe_transaction(value);
  }
   _mutex.unlock();
}
void Bank::transaction_to_every_valid_customer(
    Customer::balance_t value) {
   _mutex.lock();
  for (size_t i{}; i < _storage->user_count; ++i) {
    try {
      (*_storage)[i].transaction(value);
    } catch (const CustomerFrozenException &e) {
      std::cerr << "Customer " << i << ": " << e.what();
    } catch (const std::range_error &e) {
      std::cerr << "Customer " << i << ": " << e.what();
    }
  }
   _mutex.unlock();
}
void Bank::set_minimum_allowed(Customer::balance_t minimum_allowed,
                                       size_t index) {
   _mutex.lock();
  _storage->at(index).set_minimum_allowed(minimum_allowed);
   _mutex.unlock();
}
void Bank::set_maximum_allowed(Customer::balance_t maximum_allowed,
                                       size_t index) {
   _mutex.lock();
  _storage->at(index).set_maximum_allowed(maximum_allowed);
   _mutex.unlock();
}
