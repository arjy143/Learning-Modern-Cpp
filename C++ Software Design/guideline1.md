# Guideline 1: understand the importance of software design

Dependency is the key problem in software development at all scales.

3 levels of software devleopment:
- software architecture: client server, microservices etc
- software design: design patterns
- implementation details - make_unique, enable_if etc
Idioms sit between the layers, and involves things like design patterns (Pimpl) and implementation patterns (RAII)

Things like RAII are more related to encapsulation, not necessarily decoupling, so it fits more on the implementation detail level, not the software design level.

Try to avoid unnecessary coupling and dependencies.

c++ modules, constexpr, noexcept

[modules](/random%20code%20concepts/modules/modules.md)
[noexcept](/random%20code%20concepts/noexcept/noexcept.md)