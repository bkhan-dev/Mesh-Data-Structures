To compile run the following command in the "programs" folder:
$ make

To execute the face2faceindex program:
$ ./face2faceindex ../models/tetrahedron.tri

To execute the faceindex2directed edge program:
$ ./faceindex2directededge ../face_models/tetrahedron.face

To try different model files:
If it is a .tri file, add it to the "models" folder.
If it is a .face file, add it to the "face_models" folder.

After running the programs:
.face files are saved to the "face_models" folder.
.diredge files are saved to the "diredge_models" folder.

