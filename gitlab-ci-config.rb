HGUILD_PROJECT="libWexpr"
HGUILD_PROFILES={
	"Linux-X86_64@Clang@@" => ["Debug", "Release"],
	"macOS-ARM64@Clang@@" => ["Debug", "RelWithDebInfo"],
	"macOS-ARM64@Clang@macOS-X86_64@" => ["Debug", "RelWithDebInfo"]
}

GITLABCI_SCHEDULER="Support/gitlabScheduler.rb"
GITLABCI_SCHEDULER_PROFILE="Linux-X86_64@Clang@@"
GITLABCI_SCHEDULER_BUILDTYPE="Debug"

GITLABCI_CODEQUALITY_CLANGTIDY=true
GITLABCI_CODEQUALITY_PROFILE="Linux-X86_64@Clang@@"
GITLABCI_CODEQUALITY_BUILDTYPE="Debug"
GITLABCI_CODEQUALITY_CLANGTIDY_IGNOREFILES="*/Private/ThirdParty/*"

# disable due to C, so cant do it everywhere
GITLABCI_CODEQUALITY_CLANGTIDY_IGNORECHECKS="modernize-use-trailing-return-type;hicpp-deprecated-headers"
