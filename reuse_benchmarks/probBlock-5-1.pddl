(define (problem BLOCKS-5-1)
(:domain BLOCKS)
(:objects A B C D E)
(:INIT (ON A D) (ONTABLE B) (ON C B) (ON D E) (ONTABLE E) (CLEAR A) (CLEAR C) (HANDEMPTY))
(:goal (AND (ON A E) (ON B A) (ONTABLE C) (ON D B) (ONTABLE E) ))
)