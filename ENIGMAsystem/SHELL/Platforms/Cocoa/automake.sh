# Call this file from the directory in question if you would like a generic
# Makefile generated which invokes GCC for each individual file.

echo "#Awesome Makefile generated by ENIGMAsystem/Developer/automake.sh" > Makefile
echo "" >> Makefile;

for file in *.cpp ;
  do
  {
    printf ".eobjs_\$(MODE)/${file%.cpp}.o: $file" >> Makefile;
    for i in `c_incl $file | gawk '/\/usr\/include/ { next } { print } '`;
    do
      printf " $i" >> Makefile;
    done;
    echo "" >> Makefile;
    
    echo "	g++ -c $file		-o .eobjs_\$(MODE)/${file%.cpp}.o \$(FLAGS)"  >> Makefile;
  };
  done;

for file in *.m ;
  do
  {
    printf ".eobjs_\$(MODE)/${file%.m}.o: $file" >> Makefile;
    for i in `c_incl $file | gawk '/\/usr\/include/ { next } { print } '`;
    do
      printf " $i" >> Makefile;
    done;
    echo "" >> Makefile;
    
    echo "	g++ -c $file		-o .eobjs_\$(MODE)/${file%.m}.o \$(FLAGS)"  >> Makefile;
  };
  done;

echo "" >> Makefile;

#create the eobjs folder
echo "mkObjDir:" >> Makefile;
echo "	-mkdir .eobjs_\$(MODE)" >> Makefile;
echo "" >> Makefile;

#generate targets for each ENIGMA mode.
for modename in Run Debug Build Release;
do
  printf "$modename: mkObjDir " >> Makefile;
  for file in *.cpp ;
    do printf ".eobjs_$modename/${file%.cpp}.o " >> Makefile; 
    done;
  for file in *.m ;
    do printf ".eobjs_$modename/${file%.m}.o " >> Makefile; 
    done;
  echo "" >> Makefile;
done;

echo "" >> Makefile;
echo "clean:" >> Makefile;
echo "	\$(CREMOVE) .eobjs*\$(SLASHC)*" >> Makefile;

