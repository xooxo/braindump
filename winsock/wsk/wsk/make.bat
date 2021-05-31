@build /w /g

@del /Q *.wrn *.log *.err
@for /d %%f in ("src\objchk_*") do @rmdir /S /Q "%%f"

