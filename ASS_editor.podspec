#
# ASS Editor by CaptainAllred 2019 All rights reserved

Pod::Spec.new do |spec|

  # description
  spec.name         = "ASS_editor"
  spec.version      = "0.2.0"
  spec.summary      = "ASS subtitle format editor written complete in C"
  spec.description  = <<-DESC
    ASS Editor is a standard ASS format parsing tool written complete in C
    1. full support of any kind of ASS format
    2. override tags support
    3. high efficient, as complete in C code
  DESC
  spec.homepage     = "https://github.com/BillballSun/ASS-script-editor"
  spec.license      = "Apache License, Version 2.0"
  spec.author       = { "Bill Sun" => "captainallredbillball@gmail.com" }
  spec.source       = { :git => 'https://github.com/BillballSun/ASS-script-editor.git' }

  # target requirement
  spec.ios.deployment_target = "8.0"
  spec.osx.deployment_target = "10.11"
  spec.watchos.deployment_target = "2.0"

  # spec source
  spec.default_subspec = 'Core'
  spec.preserve_path = 'ReadMe.rtf'

  spec.subspec 'Core' do |sub|
    sub.requires_arc = false
    sub.source_files = 'Core/**/*.{h,c,m,mm}'
    sub.public_header_files = 'Core/**/*.h'
    sub.private_header_files = 'Core/**/*_Private.h'
  end

  spec.subspec 'Test' do |sub|
    sub.requires_arc = true
    sub.source_files = 'Test/**/*.{h,c,m,mm}'
    sub.public_header_files = 'Test/**/*.h'
    sub.private_header_files = 'Core/**/*_Private.h'
  end

end
