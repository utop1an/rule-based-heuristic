(define (problem BLOCKS-5-6)
(:domain BLOCKS)
(:objects A B C D E)
(:INIT (ON A B) (ON B D) (ON C E) (ON D C) (ONTABLE E) (CLEAR A) (HANDEMPTY))
(:goal (AND (ON A E) (ON B A) (ONTABLE C) (ON D B) (ONTABLE E) ))
)