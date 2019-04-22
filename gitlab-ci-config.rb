HGUILD_PROJECT="libWexpr"
HGUILD_PROFILES={
	"Linux-X86_64@Clang@@" => ["Debug", "Release"],
	"macOS-X86_64@Clang@@" => ["Debug", "Release"]
}

GITLABCI_SCHEDULER="Support/gitlabScheduler.rb"

# We use macOS due to docset generation requiring docsetutil
GITLABCI_SCHEDULER_PROFILE="macOS-X86_64@Clang@@"
GITLABCI_SCHEDULER_BUILDTYPE="Debug"

