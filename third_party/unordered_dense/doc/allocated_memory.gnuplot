#!/usr/bin/gnuplot

#set terminal pngcairo 
#set terminal pngcairo size 730,510 enhanced font 'Verdana,10'
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'

# define axis
# remove border on top and right and set color to gray
set style line 11 lc rgb '#808080' lt 1
set border 3 back ls 11
set tics nomirror
# define grid
set style line 12 lc rgb '#808080' lt 0 lw 1
set grid back ls 12

# line styles
set style line 1 lt 1 lc rgb '#1B9E77' # dark teal
set style line 2 lt 1 lc rgb '#D95F02' # dark orange
set style line 3 lt 1 lc rgb '#7570B3' # dark lilac
set style line 4 lt 1 lc rgb '#E7298A' # dark magenta
set style line 5 lt 1 lc rgb '#66A61E' # dark lime green
set style line 6 lt 1 lc rgb '#E6AB02' # dark banana
set style line 7 lt 1 lc rgb '#A6761D' # dark tan
set style line 8 lt 1 lc rgb '#666666' # dark gray


set style line 101 lc rgb '#808080' lt 1 lw 1
set border 3 front ls 101
set tics nomirror out scale 0.75

set key left top

set output 'allocated_memory.png'

set xlabel "Runtime [s]"
set ylabel "Allocated memory [MB]"

set title "Inserting 10 Million uint64\\\_t -> uint64\\\_t pairs"

# allocated_memory_segmented_vector.txt  allocated_memory_std_unordered_map.txt  allocated_memory_std_vector.txt
plot \
    'allocated_memory_segmented_vector.txt' using ($1):($2/1e6) w steps ls 1 lw 2 title "ankerl::unordered\\\_dense::segmented\\\_map" , \
    'allocated_memory_std_vector.txt' using ($1):($2/1e6) w steps ls 2 lw 2 title "ankerl::unordered\\\_dense::map" , \
    'allocated_memory_absl_flat_hash_map.txt' using ($1):($2/1e6) w steps ls 3 lw 2 title "absl::flat\\\_hash\\\_map"
