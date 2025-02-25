#!/usr/bin/env ruby

destination_dir = ARGV.fetch(0)
exit 1 if destination_dir.nil?
exit 1 if !File.exist?(destination_dir)
source_file = "assets/roll.mkv"

shows = [
  "Ulysses 31",
  "Twin Peaks",
]

episode_count = 6

files_names = shows.flat_map do |show|
  (1..episode_count).map do |episode|
    "#{show} S01E#{episode.to_s.rjust(2, '0')}.mkv"
  end
end

files_names.each do |episode_file_name|
  `cp #{source_file} "#{destination_dir}/#{episode_file_name}"`
end
