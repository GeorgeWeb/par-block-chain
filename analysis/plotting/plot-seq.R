SequentialX <- ResultsSequential$Difficulty
SequentialY <- as.numeric(as.character(ResultsSequential$`Average Time Per Block`))

sequentialFrame <- data.frame(
  SequentialX,
  SequentialY
)

cols = c("Difficulty", "Average Time Per Block")
colnames(sequentialFrame) = cols

library(ggplot2)
library(ggthemes)

p <- ggplot2::ggplot(data = sequentialFrame, aes(x = Difficulty, y = `Average Time Per Block`, fill = Difficulty, color = Difficulty, label = `Average Time Per Block`)) + 
  ggthemes::theme_economist_white() +
  geom_line() +
  labs(x = "Difficulty", y = "Time (ms)", title = "Average Time Taken (ms) vs Difficulty", subtitle = "Baseline performance measurement of the Sequential technique")
print(p)
