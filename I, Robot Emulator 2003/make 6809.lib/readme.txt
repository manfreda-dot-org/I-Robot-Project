1. copy DLL into this directory
2. run the command line "TDUMP -ee -m 6809.DLL > 6809.LST"
   to display the "true names" of the exports of a DLL
3. run "IMPDEF 6809.DEF 6809.DLL"
   this generates a generic DEF file
4. modify DEF file to rename MSVC functions into BCB functions
      ; use this type of aliasing
      ; (Borland name)   = (Name exported by Visual C++)
        _CdeclFunction   = CdeclFunction
        _UnknownFunction = UnknownFunction
5. run "make lib.bat", which copies the final lib into the parent directory
