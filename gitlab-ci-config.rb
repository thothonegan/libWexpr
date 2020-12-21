HGUILD_PROJECT="libWexpr"
HGUILD_PROFILES={
	"Linux-X86_64@Clang@@" => ["Debug", "Release"],
	"macOS-ARM64@Clang@@" => ["Debug", "RelWithDebInfo"],
	"macOS-ARM64@Clang@macOS-X86_64@" => ["Debug", "RelWithDebInfo"]
}

GITLABCI_SCHEDULER="Support/gitlabScheduler.rb"
GITLABCI_SCHEDULER_PROFILE="Linux-X86_64@Clang@@"
GITLABCI_SCHEDULER_BUILDTYPE="Debug"

