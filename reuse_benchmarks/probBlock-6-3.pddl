(define (problem BLOCKS-6-3)
(:domain BLOCKS)
(:objects A B C D E F)
(:INIT (ON A E) (ON B C) (ONTABLE C) (ONTABLE D) (ON E F) (ON F D) (CLEAR A) (CLEAR B) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) ))
)