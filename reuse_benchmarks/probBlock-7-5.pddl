(define (problem BLOCKS-7-5)
(:domain BLOCKS)
(:objects A B C D E F G)
(:INIT (ONTABLE A) (ON B A) (ON C D) (ONTABLE D) (ON E C) (ONTABLE F) (ON G B) (CLEAR E) (CLEAR F) (CLEAR G) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) ))
)