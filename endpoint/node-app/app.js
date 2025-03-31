const express = require('express');
const mysql = require('mysql2');

const app = express();
const port = process.env.PORT || 3010;

// Create a MySQL connection pool.
// Adjust the connection parameters as needed.
const pool = mysql.createPool({
  host: process.env.DB_HOST || 'db',   // Use your DB host (for Docker, this might be the service name)
  user: process.env.DB_USER || 'myuser', // Your MySQL username
  password: process.env.DB_PASSWORD || 'mypassword', // Your MySQL password
  database: process.env.DB_DATABASE || 'mydatabase',
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0,
});

// Route to display the summary of guest counts by date.
app.get('/', (req, res) => {
  const query = `
    SELECT DATE(timestamp) AS dt, SUM(count) AS total 
    FROM guest_counts 
    GROUP BY DATE(timestamp) 
    ORDER BY dt DESC
  `;
  
  pool.query(query, (err, results) => {
    if (err) {
      console.error('Error fetching data from MySQL:', err);
      return res.status(500).json({ error: err.message });
    }
    
    // Build a sleek HTML page using Tailwind CSS & dark theme.
    let html = `<!DOCTYPE html>
<html lang="en" class="dark">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Plate Summary by Date</title>
  <!-- Tailwind CSS CDN -->
  <script src="https://cdn.tailwindcss.com"></script>
  <style>
    /* Activate dark mode */
    html { color-scheme: dark; }
  </style>
</head>
<body class="bg-gray-900 text-gray-100">
  <div class="container mx-auto py-8">
    <h1 class="text-4xl font-bold mb-8 text-center">Plate Summary by Date</h1>
    <div class="space-y-6">`;
      
    results.forEach(row => {
      // Format the date to yyyy-mm-dd
      const formattedDate = new Date(row.dt).toISOString().split('T')[0];
      
      // Build the string of plate emojis. For huge numbers, you may want to limit the number of emojis displayed.
      let platesVisual = "";
      for (let i = 0; i < row.total; i++) {
        platesVisual += "🍽";
      }
      
      html += `
        <div class="bg-gray-800 rounded-lg p-4 shadow-md">
          <div class="flex justify-between items-center">
            <div class="text-xl font-semibold">${formattedDate}</div>
            <div class="text-lg">Total Plates: ${row.total}</div>
          </div>
          <div class="mt-2"><span class="text-2xl">${platesVisual}</span></div>
        </div>
      `;
    });
    
    html += `
    </div>
  </div>
</body>
</html>`;
    
    res.send(html);
  });
});

app.listen(port, () => {
  console.log(`Express app is listening on port ${port}`);
});

