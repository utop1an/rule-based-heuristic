(define (problem BLOCKS-8-5)
(:domain BLOCKS)
(:objects A B C D E F G H)
(:INIT (ON A G) (ONTABLE B) (ON C B) (ON D E) (ONTABLE E) (ON F D) (ONTABLE G) (ON H C) (CLEAR A) (CLEAR F) (CLEAR H) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) (ON H G) ))
)