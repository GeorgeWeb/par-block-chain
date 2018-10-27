TechniqueDIfficulty <- ResultsAllHardware$Difficulty
TechniqueAvgTime <- as.numeric(as.character(ResultsAllHardware$`Average Time Per Block`))
TechniqueName <- ResultsAllHardware$Technique
TechniqueSpeedup <- as.numeric(as.character(ResultsAllHardware$Speedup))
TechniqueEfficiency <- as.numeric(as.character(ResultsAllHardware$Efficiency))

allTechniquesFrame <- data.frame(
  TechniqueDIfficulty,
  TechniqueAvgTime,
  TechniqueName,
  TechniqueSpeedup,
  TechniqueEfficiency
)

cols = c("Difficulty", "Average Time Per Block", "Technique", "Speedup", "Efficiency")
colnames(allTechniquesFrame) = cols

library(ggplot2)
library(ggthemes)
library(scales)

diffifculy_labels <- c("1" = "Difficulty 1", "2" = "Difficulty 2", "3" = "Difficulty 3", "4" = "Difficulty 4", "5" = "Difficulty 5")

# Average time comparison between all methods #
pAvgTimeBar <- ggplot(data = allTechniquesFrame, aes(x = Technique, y = `Average Time Per Block`, fill = Technique)) + 
  ggthemes::theme_economist_white() +
  geom_bar(stat = "identity", position = 'stack') + 
  facet_grid(~ Difficulty, labeller=labeller(Difficulty = diffifculy_labels)) + 
  theme(strip.text.x = element_text(size=12, angle=0),
        strip.text.y = element_text(size=16, face="bold"),
        strip.background = element_rect(color="black", fill="grey")) + 
  labs(x = "Technique", y = "Time (Log2 / ms)", title = "Log of Average Time Taken (ms) vs Difficulty", subtitle = "Comparison between all methods using hardware number of threads calculation") +
  scale_x_discrete(breaks=c("0. Sequential","1. OpenMP","2. Threads + Atomics", "3. Thread Pool + Futures"), 
                   labels = c("", "", "", "")) +
  scale_y_continuous(trans = "log2")

# Speedup comparison bar graph between all parallel methods #
pSpeedupBar <- ggplot(data = subset(allTechniquesFrame,Technique != c("0. Sequential")), aes(x = Technique, y = Speedup, fill = Technique)) + 
  ggthemes::theme_economist_white() +
  geom_bar(stat = "identity", position = 'stack') + 
  facet_grid(~ Difficulty, labeller=labeller(Difficulty = diffifculy_labels)) + 
  theme(strip.text.x = element_text(size=12, angle=0),
        strip.text.y = element_text(size=16, face="bold"),
        strip.background = element_rect(color="black", fill="grey")) + 
  labs(x = "Technique", y = "Speedup factor", title = "Speedup vs Technique, Difficulty", subtitle = "Comparison between all methods using hardware number of threads calculation") +
  scale_x_discrete(breaks=c("1. OpenMP","2. Threads + Atomics", "3. Thread Pool + Futures"), 
                   labels = c("", "", ""))

# Efficiency comparison bar graph between all parallel methods #
pEfficiencyBar <- ggplot(data = subset(allTechniquesFrame,Technique != c("0. Sequential")), aes(x = Technique, y = Efficiency, fill = Technique)) + 
  ggthemes::theme_economist_white() +
  geom_bar(stat = "identity", position = 'stack') + 
  facet_grid(~ Difficulty, labeller=labeller(Difficulty = diffifculy_labels)) + 
  theme(strip.text.x = element_text(size=12, angle=0),
        strip.text.y = element_text(size=16, face="bold"),
        strip.background = element_rect(color="black", fill="grey")) + 
  labs(x = "Technique", y = "Efficiency factor", title = "Efficiency vs Technique, Difficulty", subtitle = "Comparison between all methods using hardware number of threads calculation") +
  scale_x_discrete(breaks=c("1. OpenMP","2. Threads + Atomics", "3. Thread Pool + Futures"), 
                   labels = c("", "", ""))

print(pAvgTimeBar)
#print(pSpeedupBar)
#print(pEfficiencyBar)