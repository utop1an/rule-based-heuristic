import os

prefix = "./fast-downward.py --debug --search-time-limit 15m "
enter = "--search 'idastar(h=zero(),u=true,d=false,t=1,s=true,i=true,r=false, o=false)'"
exit = "--search 'idastar(h=zero(),u=true,d=false,t=2,s=true,i=true,r=false, o=false)'"
both = "--search 'idastar(h=zero(),u=true,d=false,t=3,s=true,i=true,r=false, o=false)'"
base = "./benchmarks"
domain_suffix = "/domain.pddl"
dom = "blocks"

commands = [enter, exit, both]

instances = ["probBLOCKS-04-0.pddl", "probBLOCKS-05-0.pddl", "probBLOCKS-06-0.pddl", "probBLOCKS-07-0.pddl", "probBLOCKS-08-0.pddl"  ]

for inst in instances:
    
    for i,c in enumerate(commands):
        cmd = prefix + base + "/" + dom + domain_suffix + " " + base + "/" + dom + "/" + inst + " " + c
        with open("results.csv", mode='a') as results:
            results.write( "Type" + str(i) + "-" + inst + ",")
            results.close()
        print(cmd)
        os.system(cmd)
        
        with open("results.csv", mode='a') as results:
                results.write("\n")
                results.close()

os.rename("results.csv", "timing_results.csv")
print("Done!")

            

