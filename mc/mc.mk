##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug_Win32
ProjectName            :=mc
ConfigurationName      :=Debug_Win32
WorkspacePath          :="C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc"
ProjectPath            :="C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=vhanla
Date                   :=20/08/2016
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :="C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/link.exe" -nologo
SharedObjectLinkerName :="C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/link.exe" -DLL -nologo
ObjectSuffix           :=.obj
DependSuffix           :=
PreprocessSuffix       :=.i
DebugSwitch            :=/Zi 
IncludeSwitch          :=/I
LibrarySwitch          := 
OutputSwitch           :=/OUT:
LibraryPathSwitch      :=/LIBPATH:
PreprocessorSwitch     :=/D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName).exe
Preprocessors          :=$(PreprocessorSwitch)WIN32 $(PreprocessorSwitch)_DEBUG $(PreprocessorSwitch)_WINDOWS 
ObjectSwitch           :=/Fo
ArchiveOutputSwitch    :=/OUT:
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="mc.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  -O0
IncludePath            := $(IncludeSwitch)"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include"  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                :=$(LibraryPathSwitch)"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib"  $(LibraryPathSwitch). $(LibraryPathSwitch). $(LibraryPathSwitch)Debug 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/lib.exe" -nologo
CXX      := "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe" -nologo -FC -EHs
CC       := "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe" -nologo -FC
CXXFLAGS :=  -g -Wall $(Preprocessors)
CFLAGS   :=   $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
VS_Configuration:=Debug
VS_IntDir:=./Debug/
VS_OutDir:=./Debug/
VS_Platform:=Win32
VS_ProjectDir:=C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/
VS_ProjectName:=mc
VS_SolutionDir:=C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/
Objects0=$(IntermediateDirectory)/MainWindow.cpp$(ObjectSuffix) $(IntermediateDirectory)/mc.cpp$(ObjectSuffix) $(IntermediateDirectory)/McAnimation.cpp$(ObjectSuffix) $(IntermediateDirectory)/McPiles.cpp$(ObjectSuffix) $(IntermediateDirectory)/McProps.cpp$(ObjectSuffix) $(IntermediateDirectory)/ThumbWindow.cpp$(ObjectSuffix) $(IntermediateDirectory)/WindowItem.cpp$(ObjectSuffix) $(IntermediateDirectory)/WindowList.cpp$(ObjectSuffix) $(IntermediateDirectory)/ZoomWindow.cpp$(ObjectSuffix) $(IntermediateDirectory)/mc.rc$(ObjectSuffix) \
	



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Debug"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/MainWindow.cpp$(ObjectSuffix): MainWindow.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/MainWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/MainWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/MainWindow.cpp$(PreprocessSuffix): MainWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/MainWindow.cpp$(PreprocessSuffix)MainWindow.cpp

$(IntermediateDirectory)/mc.cpp$(ObjectSuffix): mc.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/mc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mc.cpp$(PreprocessSuffix): mc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mc.cpp$(PreprocessSuffix)mc.cpp

$(IntermediateDirectory)/McAnimation.cpp$(ObjectSuffix): McAnimation.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/McAnimation.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/McAnimation.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/McAnimation.cpp$(PreprocessSuffix): McAnimation.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/McAnimation.cpp$(PreprocessSuffix)McAnimation.cpp

$(IntermediateDirectory)/McPiles.cpp$(ObjectSuffix): McPiles.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/McPiles.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/McPiles.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/McPiles.cpp$(PreprocessSuffix): McPiles.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/McPiles.cpp$(PreprocessSuffix)McPiles.cpp

$(IntermediateDirectory)/McProps.cpp$(ObjectSuffix): McProps.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/McProps.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/McProps.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/McProps.cpp$(PreprocessSuffix): McProps.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/McProps.cpp$(PreprocessSuffix)McProps.cpp

$(IntermediateDirectory)/ThumbWindow.cpp$(ObjectSuffix): ThumbWindow.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/ThumbWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ThumbWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ThumbWindow.cpp$(PreprocessSuffix): ThumbWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ThumbWindow.cpp$(PreprocessSuffix)ThumbWindow.cpp

$(IntermediateDirectory)/WindowItem.cpp$(ObjectSuffix): WindowItem.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/WindowItem.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/WindowItem.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/WindowItem.cpp$(PreprocessSuffix): WindowItem.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/WindowItem.cpp$(PreprocessSuffix)WindowItem.cpp

$(IntermediateDirectory)/WindowList.cpp$(ObjectSuffix): WindowList.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/WindowList.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/WindowList.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/WindowList.cpp$(PreprocessSuffix): WindowList.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/WindowList.cpp$(PreprocessSuffix)WindowList.cpp

$(IntermediateDirectory)/ZoomWindow.cpp$(ObjectSuffix): ZoomWindow.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/ZoomWindow.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ZoomWindow.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ZoomWindow.cpp$(PreprocessSuffix): ZoomWindow.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ZoomWindow.cpp$(PreprocessSuffix)ZoomWindow.cpp

$(IntermediateDirectory)/mc.rc$(ObjectSuffix): mc.rc
	$(RcCompilerName) -i "C:/Users/vhanla/Documents/Visual Studio 2015/Projects/mc/mc/mc.rc" $(RcCmpOptions)   $(ObjectSwitch)$(IntermediateDirectory)/mc.rc$(ObjectSuffix) $(RcIncludePath)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


