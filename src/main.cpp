#include "Application.hpp"

int main() {
    Application app;
    app.Init();
    app.Run();
    app.Shutdown();
    return 0;
}