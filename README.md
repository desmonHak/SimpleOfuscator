
```c
E:\SimpleOfuscator>gcc -I . ./LZ77_c/LZ77.c code2.c -o code2.exe
```

```c
E:\SimpleOfuscator>code2.exe o code2.c code3.c "1234hola mundo"
Entropia de los datos de entrada: 4.766296
Entropia de los datos de salida: 7.676748
Archivo ofuscado y comprimido guardado en: code3.c

E:\SimpleOfuscator>code2.exe d code3.c code4.c "1234hola mundo"
Entropia de los datos de entrada: 7.676748
Archivo descomprimido y desofuscado guardado en: code4.c

```