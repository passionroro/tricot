#include <iostream>

int runInperpreter(int argc, char **argv);
int runInperpreter();
int runColorsDetector();

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <interpret|read> [arguments...]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "interpret")
    {
        if (argv[2])
        {
            runInperpreter(argc - 1, argv + 1);
        }
        else
        {
            runInperpreter();
        }
    }
    else if (mode == "read")
    {
        runColorsDetector();
    }
    else
    {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

    return 0;
}
