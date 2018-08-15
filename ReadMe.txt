Auro

===========================================================================

Para este proyecto he tomado el atlas de textura del personaje principal del juego Prince of Persia 2: The Shadow and the Flame (1993) y lo he animado, haciéndolo obedecer tres comandos de voz: “anda”, “gira” y “salta”.

No he utilizado ninguna librería de reconocimiento de voz, todo el código está en el proyecto. Me he apoyado en el código fuente del programa AurioTouch de Apple, un clásico para todo el que quiera iniciarse en el procesamiento de señales, así como en las clases de ayuda en C++ de iPublicUtility, también de Apple.

El núcleo es un algoritmo de transformadas rápidas de Fourier rádix 4 en ensamblador (ha sido hermoso aprender a progamar en ensamblador). Para el análisis de los datos de entrada (obtención de formantes, etc.) he utilizado predicción lineal, gracias a la cual obtengo las 5 formantes de las vocales con una precisión bastante buena. Se me resisten ciertos sonidos vocálicos de las cerradas posteriores. Con las voces femeninas funciona mejor pero con las masculinas al programa le cuesta detectar las formantes de frecuencias bajas.

He usado un iPhone 4 y, para el núcleo del algoritmo, el Accelerate Framework de Apple. He utilizado el algoritmo de Burg tal y como lo explica Cedrick Collomb y también he estudiado el código fuente del software de análisis de voz Praat para ciertas técnicas.

Combino las formantes F1 y F2 para obtener los resultados. He usado los valores medios para ambas formantes que da J. C. Catford en A Practical Introduction to Phonetics para ubicar las vocales.

He probado la aplicación en la calle y demuestra ser bastante resistente al ruido, incluso al del tráfico.

Pedro Pinto.
