(define (problem BLOCKS-7-4)
(:domain BLOCKS)
(:objects A B C D E F G)
(:INIT (ON A D) (ON B G) (ON C E) (ON D F) (ONTABLE E) (ON F B) (ON G C) (CLEAR A) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) ))
)