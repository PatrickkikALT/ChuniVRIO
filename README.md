# ChuniIO
Allows connection to Chunithm via Shared Memory. No LED functionality yet.

To use simply drop into the Chunithm folder then open segatools.ini and change this line: <br>
`[chuniio]` <br>
`; Uncomment this if you have custom chuniio implementation comprised of a single 32bit DLL.` <br>
`; (will use chu2to3 engine internally)` <br>
`path=chuniio.dll`

## Development
This IO opens up the input through a Shared Memory link. <br>
Simply write a 34 size byte array to "ChuniIOSharedMemory" where the byte[0] is an operator button, byte[1] beams broken, and byte[2 to 34] the pressure value per cell.
