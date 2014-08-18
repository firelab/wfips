library(rpart)
#setwd("G:\\Workspace\\FIG-tree")
#xmat <- read.csv("IsaacFireDay2-valid.csv")
#DayOfWeek <- as.factor(xmat[,2])
#Jday <- as.numeric(xmat[,4])
#Nfires <- as.numeric(xmat[,5])
#MaxROS <- as.numeric(xmat[,6])
#AvgROS <- as.numeric(xmat[,7])
#IsContained <- as.factor(as.logical(xmat[,8] == " Contained"))
#tree.out <- rpart(IsContained ~ DayOfWeek + Jday + Nfires + MaxROS + AvgROS, 
#	control=rpart.control(minsplit=50, cp=0.01),
#	parms = list(loss = matrix(c(0, 1, 40, 0), nrow = 2)), method = "class")
#png("fig-tree1.png", width=800, height=800)
#plot(tree.out, uniform=TRUE, 
#  	 main="Classification Tree for Fire Days")
#text(tree.out, use.n = TRUE, all=TRUE, cex=.8)
#dev.off()

xmat <- read.csv("fires.csv")
ArrTime <- as.numeric(xmat[,14])
ArrSize <- as.numeric(xmat[,15])
ROS <- as.numeric(xmat[,8])
FuelModel <- as.factor(xmat[,9])
IsContained <- as.numeric(xmat[,16] == "Contained")

tree.out <- rpart(IsContained ~ ArrTime + ArrSize + ROS + FuelModel, control=rpart.control(minsplit=50, cp=0.01), method = "class")
png("fig-tree.png", width=800, height=800)
plot(tree.out, uniform=TRUE, main="Classification Tree for Fire Containment")
text(tree.out, use.n = TRUE, all=TRUE, cex=.8)
dev.off()






