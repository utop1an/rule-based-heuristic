(define (problem BLOCKS-7-9)
(:domain BLOCKS)
(:objects A B C D E F G)
(:INIT (ONTABLE A) (ON B D) (ON C F) (ONTABLE D) (ON E B) (ONTABLE F) (ONTABLE G) (CLEAR A) (CLEAR C) (CLEAR E) (CLEAR G) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) ))
)