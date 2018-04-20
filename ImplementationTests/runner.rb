#!/usr/bin/env ruby

tests = []

scriptDir = File.expand_path(File.dirname(__FILE__))

commandToRun = ""
displayOutput = false
ARGV.each do|a|
	if a == "--displayOutput"
		displayOutput = true
		next
	end

	commandToRun += ('"' + a + '"' + " ")
end

if commandToRun == "" or ! commandToRun.include? "{}"
	puts ">> Pass the command to run with {} for where to put input files"
	puts ">> for example: ./runner.rb WexprTool -c validate -i {}"
	exit 1
end

Dir["#{scriptDir}/success/*.wexpr"].each do |file|
	tests << {
		fileName: file,
		shouldBeCorrect: true
	}
end

Dir["#{scriptDir}/fail/*.wexpr"].each do |file|
	tests << {
		fileName: file,
		shouldBeCorrect: false
	}
end

pass=0
fail=0
ran=0

tests.each do |testData|
	fileName = testData[:fileName]
	shortFileName = fileName.sub "#{scriptDir}/", ""
	
	puts "#{ran+1}) #{shortFileName}..."
	
	cmd=commandToRun.sub '{}', fileName
	
	output = `#{cmd} 2>&1`
	
	if displayOutput
		puts output
	end

	if $?.success? == testData[:shouldBeCorrect]
		pass += 1
	else
		puts ""
		puts "!!! FAIL: #{fileName} : got #{$?.success?} but expected #{testData[:shouldBeCorrect]}"
		puts ""
		fail += 1
	end
	
	ran += 1
end

puts "---"
puts "Pass: #{pass} (#{(pass*1.0/(ran)*100).to_i}%)"
puts "Fail: #{fail} (#{(fail*1.0/(ran)*100).to_i}%)"
puts "Total: #{ran}"
