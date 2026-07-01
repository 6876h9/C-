#include "driver.h"

int main(int argc, char** argv) {
    cminus::CompilerOptions opts;
    if (!cminus::parse_args(argc, argv, opts)) {
        return 1;
    }
    return cminus::compile(opts);
}
