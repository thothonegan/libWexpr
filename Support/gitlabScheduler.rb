#!/usr/bin/env ruby

# gitlabScheduler.rb
# Performs jobs that we want to run on schedules
#
# Variables:
# - SCHEDULER_JOB: Name of the job [valid: build_docset]
#
# - HGUILD_PROFILE: Provided automatically, the profile we're running in
# - HGUILD_BUILDTYPE: Provided automatically, the buildtype we're using.
# - HGUILD: Provided automatically, Our hguild tool
# - HGUILD_PROJECT_NAME: Provided automatically, The project name we're building
# - HGUILD_SOURCENAME: Provided automatically, the source to use such as 'Wolf@CI'.

# Globals to change if moving this file to another project
IDENTIFIER = "com.hackerguild.libwexpr"
CMAKE_TARGET = "doxygen"

# --- main

schedulerJob = ENV['SCHEDULER_JOB']

rootDir = File.expand_path("..", __dir__)

def runProcess(args)
	puts "> -------- #{args}"
	
	isSuccess = system(args)
	
	puts "< -------- #{args}"
	
	return isSuccess
end

# JOB: 'build_docset' - Will run doxygen, build the docset, and upload it to developer.hackerguild.
if schedulerJob == "build_docset"
	hguild = ENV['HGUILD']
	
	project = ENV['HGUILD_PROJECT_NAME']
	sourceName = ENV['HGUILD_SOURCENAME']
	
	dateFilename = `date "+%Y-%m-%d_%H-%M-%S"`.strip
	
	buildPath = `#{hguild} source:buildPath --buildType #{ENV['HGUILD_BUILDTYPE']} --profile #{ENV['HGUILD_PROFILE']} --customSourceDir='#{rootDir}' #{sourceName}`.strip
	htmlPath = "#{buildPath}/Documentation/Doxygen/html"
	
	# we assume we're on linux, ninja, and will be using the normal @CI build.
	# so order of operations:
	# - have cmake run doxygen, generating the html folder
	runProcess("cmake --build #{buildPath} --target #{CMAKE_TARGET}") or abort ("Unable to run cmake build")
	
	# - run make to generate the docset
	runProcess("make -C '#{htmlPath}' docset > /dev/null") or abort ("Unable to make docset")
	
	# - generate icons
	if File.exist? "#{rootDir}/logo.png"
		runProcess("convert #{rootDir}/logo.png -resize 16x16 #{htmlPath}/#{IDENTIFIER}.docset/icon.png") or abort ("Unable to create icon")
		runProcess("convert #{rootDir}/logo.png -resize 32x32 #{htmlPath}/#{IDENTIFIER}.docset/icon@2x.png") or abort ("Unable to create icon @2x")
	end
	
	# - package the docset
	outFile = "#{IDENTIFIER}.#{dateFilename}.docset.tgz"
	runProcess("cd #{htmlPath} && tar --exclude='.DS_Store' -cvzf #{outFile} #{IDENTIFIER}.docset") or abort ("Failed to package docset")
	
	# - upload the docset
	
	runProcess ("curl -fsSL 'http://developer.hackerguild/Documentation/api/documentationUpload.rb' | ruby -- - #{project} #{dateFilename} #{IDENTIFIER}.docset #{htmlPath}/#{outFile}") or abort ("Failed to upload")
	
	# - delete the html folder and recreate empty
	runProcess("rm -Rf #{htmlPath} && mkdir #{htmlPath}") or abort ("Failed to cleanup")
	
else
	abort "Unknown SCHEDULER_JOB to do - doing nothing"
end
