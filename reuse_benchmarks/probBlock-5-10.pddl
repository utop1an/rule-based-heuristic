(define (problem BLOCKS-5-10)
(:domain BLOCKS)
(:objects A B C D E)
(:INIT (ONTABLE A) (ON B C) (ONTABLE C) (ON D B) (ON E D) (CLEAR A) (CLEAR E) (HANDEMPTY))
(:goal (AND (ON A E) (ON B A) (ONTABLE C) (ON D B) (ONTABLE E) ))
)