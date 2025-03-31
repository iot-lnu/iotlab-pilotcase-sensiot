# Use an official Node.js runtime as the base image
FROM node:14-alpine

# Create and change to the app directory
WORKDIR /usr/src/app

# Copy package.json and package-lock.json (if available)
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy the rest of the application
COPY . .

# Expose port 3010
EXPOSE 3010

# Run the application
CMD ["node", "app.js"]

