# WSLDistributionFlags

An utility to adjust WSL distribution flags.

## How to use?

Compile the program using Visual Studio 2022, or download from release.

## Why I made this?

In my WSL Ubuntu-24.04, I can't write to `/proc/sys/fs/binfmt_misc/WSLInterop`. Not sure whether it's a bug, but it just shows permission denied, so that I'm unable to use software like [cosmopolitan](https://github.com/jart/cosmopolitan).

However, there's an alternative solution. We can remove the `WSL_DISTRIBUTION_FLAGS_ENABLE_INTEROP` flag in WSL distribution flags. Therefore, I wrote the utility, which can also adjust other flags.
