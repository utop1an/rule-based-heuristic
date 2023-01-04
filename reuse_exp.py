import os

prefix = "./fast-downward.py --debug --search-time-limit 15m "
hd = "--search 'idastar(h=zero(),u=true,d=false,t=false,s=false,i=true, "
base = "./reuse_benchmarks"
domain_suffix = "/domain.pddl"





instances = os.listdir(base+"/")
instances.remove("domain.pddl")
instances.sort()

count =0

cmd = prefix + base  + domain_suffix + " " + base +  "/" + instances[0] + " " + hd +  "r=false, o=true)'"
with open("results.csv", mode='a') as results:
    results.write( instances[0] + ",")
    results.close()
os.system(cmd)

with open("results.csv", mode='a') as results:
        results.write("\n")
        results.close()

cmd = prefix + base  + domain_suffix + " " + base +  "/" + instances[0] + " " + hd + "r=true, o=false)'"
with open("results.csv", mode='a') as results:
    results.write( instances[0] + " with reuse,")
    results.close()
os.system(cmd)

with open("results.csv", mode='a') as results:
        results.write("\n")
        results.close()
instances = instances[1:]
for inst in instances:
    if (inst!="domain.pddl"):
        cmd = prefix + base  + domain_suffix + " " + base + "/" + "/" + inst + " " + hd + "r=false, o=false)'"
        with open("results.csv", mode='a') as results:
            results.write( inst + ",")
            results.close()
        os.system(cmd)
        
        with open("results.csv", mode='a') as results:
                results.write("\n")
                results.close()
        
        cmd = prefix + base  + domain_suffix + " " + base +  "/" + inst + " " + hd + "r=true, o=false)'"
        with open("results.csv", mode='a') as results:
            results.write( inst + " with reuse,")
            results.close()
        os.system(cmd)
        
        with open("results.csv", mode='a') as results:
                results.write("\n")
                results.close()
    
os.rename("results.csv", "reuse_results.csv")
    
print("Done!")

            

