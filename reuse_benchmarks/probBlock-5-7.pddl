(define (problem BLOCKS-5-7)
(:domain BLOCKS)
(:objects A B C D E)
(:INIT (ON A E) (ONTABLE B) (ONTABLE C) (ON D B) (ONTABLE E) (CLEAR A) (CLEAR C) (CLEAR D) (HANDEMPTY))
(:goal (AND (ON A E) (ON B A) (ONTABLE C) (ON D B) (ONTABLE E) ))
)