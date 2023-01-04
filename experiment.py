import os

prefix = "./fast-downward.py --debug --search-time-limit 15m "
hd = "--search 'idastar(h=zero(),u=true,d=false,t=false,s=true,i=true)'"
hmax = "--search 'idastar(h=hmax(),u=false,d=false,t=false,s=false,i=false)'"
hlmcut = "--search 'idastar(h=lmcut(),u=false,d=false,t=false,s=false,i=false)'"
base = "./benchmarks"
domain_suffix = "/domain.pddl"

domains = os.listdir(base)

commands = [(hd,"hd"), (hmax, "hmax"), (hlmcut, "hlmcut")]

for c in commands:

    for dom in domains:
        instances = os.listdir(base+"/"+dom)
        instances.sort()
        instances = instances[0:21]
        count =0
        for inst in instances:
            if (inst!="domain.pddl"):
                cmd = prefix + base + "/" + dom + domain_suffix + " " + base + "/" + dom + "/" + inst + " " + c[0]
                with open("results.csv", mode='a') as results:
                    results.write(dom + "/" + inst + ",")
                    results.close()
                os.system(cmd)
                
                with open("results.csv", mode='a') as results:
                        results.write("\n")
                        results.close()
    os.rename("results.csv", c[1]+"_results.csv")
print("Done!")

            

